# VidRe I/O architecture and naming

> **Status:** AI-assisted design discussion record, not a normative API specification.  
> **Location:** `docs/ai/chatgpt/io-architecture-and-naming.md`  
> **Last consolidated:** 2026-10-15  
> **Authority:** Current source code defines implemented behavior; maintained architecture/design documentation defines design intent. This document records the discussion and its rationale. Verify all implementation claims against the current tree before treating them as facts.

## Purpose and scope

VidRe is a cross-platform C++ video-recording framework for scientific instrumentation. Its I/O layer is conceived as a graph of packet-processing endpoints. This note preserves the architecture and naming discussions, particularly decisions whose rationale may not be obvious from code alone.

Status terms used below:

- **Reported implemented:** code or behavior described as working during the discussion; not independently checked against the current source snapshot.
- **Design decision:** an agreed direction, which may be partially implemented.
- **Proposal / deferred:** discussed but not established as implemented.
- **Open:** requires source review or further design work.

## 1. Endpoint model and graph topology

### 1.1 Roles versus concrete endpoint classes

**Design decision.** `io::endpoint` is the common endpoint abstraction, with nested role bases `endpoint::source` and `endpoint::sink`. These represent the ability to produce or consume packets, respectively. The role bases virtually inherit `endpoint`.

The ordinary `io::source` and `io::sink` classes are concrete one-to-one endpoint implementations, rather than synonyms for the abstract roles. `io::channel` combines both directions:

```cpp
class channel : public io::source, public io::sink { /* ... */ };
```

The distinction matters for fan endpoints, which expose a role at the graph boundary while delegating individual connections to branch objects.

**Rationale.** Roles express connection eligibility without forcing every source or sink to have one-to-one topology. Keeping `io::source` / `io::sink` as the ordinary implementations permits reusable connection and packet-handling behavior.

### 1.2 Connections and branch normalization

**Design decision / reported implementation.** Connections are established through a central operation conceptually of the form:

```cpp
io::endpoint::connect(src, snk, settings::json config);
```

Before connecting, the operation resolves endpoints that need per-connection branches. Role-specific hooks such as `branch_source()` and `branch_sink()` provide those branches; the default behavior is to return no branch. The resolved endpoints are then configured, checked for compatible capabilities, and attached.

This avoids teaching ordinary endpoints how fan endpoints are implemented. A fan's public graph-facing object can accept connections while each edge ultimately attaches to an ordinary one-to-one branch.

**Reported lifecycle vocabulary:**

- `connect`: establish a connection after branch resolution, setup, and capability negotiation.
- `attach`: establish the concrete peer relationship.
- `detach`: notify a peer that its counterpart is going away.
- `unlink`: disconnect an endpoint from the graph.

**Reported behavior.** When a channel is unlinked and has both a predecessor and a successor, the neighboring endpoints can be reconnected. Fan branches instead remove their corresponding branch; they do not imply channel-style bypass reconnection.

**Open:** Exact failure/rollback semantics if setup, capability negotiation, or attachment fails should be checked in source and documented there.

### 1.3 Configuration belongs to participating endpoints

**Design decision.** The `connect` configuration is a JSON object passed to both participating endpoints via `setup(config)`. Configuration sections are namespaced by consumer; endpoints ignore unrelated sections.

Example from the logging discussion:

```cpp
auto outp = io::endpoint::make<store>();
io::endpoint::connect(log::output, outp,
                      {{"log", {{"threshold", "status"}}}});
log::notice() << "This is a notice message.\n";
outp->unlink();
```

**Rationale.** The connection operation need not know the configuration schema of every endpoint subtype. Passing a shared configuration and letting endpoints consume their own sections is simpler than central dispatch or a global schema.

**Caveat:** The example reflects a reported working logging/fan-out integration, not a promise that the exact signatures remain unchanged.

## 2. Fan-out and fan-in

### 2.1 Fan-out

**Reported implemented.** `io::fan::out` combines a sink implementation with a source role. Each outgoing connection is represented by a `branch` behaving as an `io::source`.

Conceptually:

```text
upstream -> fan::out -> branch (source) -> sink A
                    -> branch (source) -> sink B
```

The fan object receives packets and distributes them to the branches. The branch is the concrete one-to-one endpoint visible to the downstream sink.

**Rationale.** A single source role can support multiple edges without weakening the one-to-one invariants of ordinary endpoints.

### 2.2 Fan-in

**In development during discussion.** `io::fan::in` is the mirror: it exposes a sink role for multiple incoming connections, with each incoming edge represented by a branch behaving as an `io::sink`. The fan provides the downstream source behavior.

The discussed policy uses a queue per branch and round-robin selection among branches. It does **not** sort packets globally by timestamp.

**Rationale.** Round-robin scheduling avoids imposing timestamp comparability, synchronization, or ordering assumptions on scientific packet streams. Timestamp-ordered merging would be a separate, more specialized behavior.

### 2.3 Queue and synchronization details

**Discussed implementation approach:**

- Each fan-in branch owns a packet queue protected by a mutex.
- For a depth-one queue, the existing front element may be replaced under lock using `queue_.front() = std::move(pkt)`; `std::queue::front()` returns a reference.
- Branch collections need synchronization when connections may be unlinked concurrently.
- A `std::shared_mutex` can permit concurrent read access and exclusive modification, but a round-robin cursor is mutable state and must **not** be updated unsafely under only a shared lock.
- Copying a `std::shared_ptr` to a branch while holding the collection lock can preserve branch lifetime after releasing that lock; this does not by itself establish safety for branch internal state.

**Open:** Confirm queue capacity/drop policy, fairness guarantees, wakeup/notification behavior, shutdown ordering, and the precise lock hierarchy against the implementation. Avoid specifying stronger concurrency guarantees here than the code provides.

## 3. Packet identity, representation, and decoding

### 3.1 Major/minor identifiers

**Reported recent design.** A packet ID consists of two 8-bit components:

```cpp
struct packet::id {
    uint8_t major;
    uint8_t minor;
    // ...
};
```

`io::packet::major` defines known major categories, with an extension path for user-defined major types. Minor IDs are defined by the implementation of the corresponding major packet family. A stream identifier is **separate** from the major/minor packet ID.

The discussion mentioned `control`, `generic` (with `generic = 0x02`), `log`, and a reserved `invalid` value. The precise enumerator set and numeric assignments must be verified in the current headers.

**Rationale.** Major/minor IDs describe packet type; stream identity describes provenance/routing context. Conflating the two would unnecessarily bind packet semantics to graph topology.

### 3.2 Compatible ID types and casting

**Recent naming decision.** Prefer the concept name `compatible_id` over the earlier `packet_code` terminology. The concept lives in `vidre::io`, not a nested `packet::details` namespace. Conversion is expressed as `packet::id::cast` rather than a free-standing `code_cast` helper.

The intended accepted inputs are ordinary integral values (including plain integer literals such as `0`) and appropriately constrained unsigned enumeration types. Integral values are range-checked before narrowing to `uint8_t`.

Illustrative form from the discussion:

```cpp
template <compatible_id T>
static constexpr uint8_t cast(T value) {
    if constexpr (not std::is_enum_v<std::remove_cvref_t<T>>) {
        if (not std::in_range<uint8_t>(value))
            throw exception::logic() << "partial packet ID exceeds uint8_t range";
    }
    return static_cast<uint8_t>(value);
}
```

This is **illustrative**, not a verified source excerpt. Skipping the range check for enums is safe only if `compatible_id` actually constrains their underlying representation so that narrowing cannot lose information.

**Contract discussion.** `std::in_range<uint8_t>(value)` includes `0xFF`: it checks representability, not semantic validity. A reserved ID may be excluded by the API contract without adding a runtime check. The conversation explicitly distinguished language-level C++ undefined behavior from behavior left unspecified by an API contract; do not conflate them in formal documentation.

**Naming rationale.** `compatible_id` describes a type's suitability as a major or minor ID component without introducing a separate notion of a packet “code.” `packet::id::cast` keeps conversion close to the type that owns the representation.

### 3.3 Packet decoding and registration

**Reported architecture.** `packet::decode()` is the public entry point for reconstructing a serialized packet. An internal factory/registry mechanism dispatches by packet identity; `packet::registry<T>` provides self-registration for packet implementations.

**Naming discussion.** The internal “factory” terminology might be replaced with “registry” where it primarily stores constructors keyed by ID. No final rename was established in the discussion.

**Deferred:** The semantics and use cases of the `generic` major category were intentionally postponed.

**Source-review requirement:** Verify serialized header layout, TLV details, endian conventions, failure behavior, and registry lifetime from code. Those are not specified here.

## 4. Logging as an I/O use case

**Reported working integration.** Logging uses the packet I/O graph rather than a wholly separate output pipeline. The discussion used these names:

| Name | Intended responsibility |
| --- | --- |
| `log::message` | Message-building base, previously discussed as `log::handler` |
| `log::error`, `log::notice`, etc. | Convenient severity-specific message builders |
| `log::source` | Logging source type, previously named `log::output` |
| `log::output` | Global logging output instance, previously named `log::source` |
| `log::packet` | Packet carrying a log message |

**Naming rationale.** The type describes what it is (`source`); the global instance describes its purpose (`output`). The builder is a `message`, not a `handler`, because it constructs/emits a message rather than handling arbitrary incoming events.

**Proposals / deferred:** Converting log packets to text chunks using `plain`, `tagged`, or `colored` formatting; a `log::file` sink building on `io::file::sink` with specialized setup such as rotation. These should not be represented as completed features without source verification.

## 5. Topology orchestration: switchboard, not router

**Conceptual proposal, not established as implemented.** An `io::switchboard` could own and manage desired graph topology, including connections, disconnections, and future GUI editing. It would invoke the existing endpoint connection machinery rather than route packets itself.

**Naming rationale.** “Switchboard” suggests connection management and topology orchestration. “Router” suggests runtime packet forwarding or route selection, which is not the proposed responsibility. A graphical editor may expose internal fan branches while preserving the distinction between user-visible topology and concrete connection endpoints.

**Rejected architectural direction:** Do not introduce a general packet router merely to centralize topology edits. Keep data movement with endpoints and graph management with an optional orchestration layer.

## 6. Naming principles and alternatives

The discussion repeatedly favored names that encode actual responsibilities rather than implementation mechanics or premature abstractions.

| Preferred / current direction | Earlier or competing term | Reasoning / status |
| --- | --- | --- |
| `endpoint::source`, `endpoint::sink` | Using `io::source` / `io::sink` for every role | Distinguish abstract capability from ordinary one-to-one implementation; design decision |
| `branch_source()`, `branch_sink()` | Implicit fan special cases in `connect()` | Make per-edge normalization role-specific; reported design |
| `connect`, `attach`, `detach`, `unlink` | A single overloaded connection verb | Separate orchestration, concrete attachment, peer notification, and graph removal; reported vocabulary |
| `log::message` | `log::handler` | Message construction rather than generic handling; reported rename |
| `log::source` type / `log::output` instance | Opposite assignment | Separate type identity from instance purpose; reported rename |
| `compatible_id` | `packet_code` | Avoid introducing unnecessary “code” terminology; recent decision |
| `packet::id::cast` | `code_cast` | Keep conversion with the owning representation; recent decision |
| `switchboard` | `router` | Topology orchestration, not packet routing; proposal |
| Internal `registry` | Internal `factory` | Potentially better description of ID-to-constructor registration; unresolved |

A broader theme was to avoid introducing a new “junction” abstraction solely to unify fan and channel topology where composition of existing endpoint roles is sufficient. The exact scope of any rejected `junction` design should be recovered from the original discussion if a formal ADR is needed.

## 7. Concurrency and lifecycle design cautions

These are implications of the discussed design, **not verified guarantees**:

1. A fan's branch list and each branch's queue have different synchronization needs. Protecting one does not automatically protect the other.
2. A `shared_mutex` is useful only if readers truly remain read-only. Round-robin selection may mutate state even while traversing a read-only collection.
3. Unlink and destruction must coordinate with in-flight packet delivery and branch selection. A retained `shared_ptr` can protect object lifetime, but not prevent logically stale delivery unless the contract allows it.
4. Reconnection during `channel::unlink()` changes graph topology and deserves explicit lock-order and callback/reentrancy review.
5. Configuration and capability negotiation should have well-defined failure behavior so partial connection setup cannot leave an inconsistent graph.

These concerns are candidates for targeted tests and source comments, rather than justification for redesigning the graph model prematurely.

## 8. Outstanding questions / source verification checklist

- [ ] Verify current class declarations and virtual inheritance for endpoint roles, concrete endpoints, channels, and fans.
- [ ] Verify exact `connect`/`setup`/capability/`attach` ordering and rollback behavior.
- [ ] Verify `unlink`/`detach` semantics and concurrency guarantees, including channel bypass behavior.
- [ ] Verify fan-in scheduling, branch queue policy, mutex ownership, and round-robin cursor synchronization.
- [ ] Verify fan-out delivery and per-branch error/backpressure behavior.
- [ ] Verify packet ID sizes, major enumerators, reserved IDs, and `compatible_id` constraints.
- [ ] Verify `packet::decode()` and `packet::registry<T>` APIs, dispatch ownership, and duplicate registration behavior.
- [ ] Decide whether internal `factory` should be renamed `registry`.
- [ ] Define or explicitly leave deferred the semantics of the `generic` packet major.
- [ ] Decide whether a `switchboard` is needed, and only then specify ownership and graph mutation APIs.
- [ ] Check which proposed log formatting/file-sink features have been implemented.

## 9. How to maintain this record

This file is an **AI discussion archive** and design handover. It should not compete with maintained architecture documents or Doxygen API comments.

When a decision is implemented or revised:

1. Update the authoritative source and its nearby documentation.
2. Update the maintained architecture/decision record, including why alternatives were rejected.
3. Add a short dated note here if the discussion materially influenced the choice.
4. Avoid silently rewriting historical rationale to make a later design appear inevitable.

### Provenance

Consolidated from the long VidRe ChatGPT architecture/naming discussion, particularly the endpoint/fan topology, logging integration, packet-ID refactoring, and registry discussions. The conversation also contained exploratory suggestions and incomplete implementation snapshots; this document deliberately marks those separately from reported working behavior. No current repository source snapshot was available for independent verification during this consolidation.

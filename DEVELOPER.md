# Developer info

TODO

cmake:
- VIDRE_DEVELOP
- VIDRE_BUILD_TESTS
- VIDRE_BUILD_EXAMPLES
- VIDRE_BUILD_GUI


## GIT

### Feature implementation

Each feature should be developed in a separate branch.

Branch naming convention: `{initials}/{category}/{descriptive-word}`

For work involving multiple developers, use `feature` instead of the developer's initials.

Example: `js/io/packet` or `feature/io/packet`.

**1. Create a feature branch**

Start from an up-to-date `main` branch:

```bash
git switch main
git pull --ff-only
git switch -c js/io/packet
```

**2. Push the branch upstream**

```bash
git push -u origin js/io/packet
```

**3. Implement the feature**

Work on the branch, committing and pushing changes as needed:

```bash
git add .
git commit -m "Implement packet handling"
git push
```

**4. Merge into main**

Once implementation and testing are complete, merge the feature branch into `main`:

```bash
git switch main
git pull --ff-only
git merge --squash js/io/packet
git commit -m "Implement packet handling"
git push
```

Using `--squash` combines the feature's changes into a single commit on `main`, without preserving the individual feature-branch commits in its history.

**5. Remove the remote branch**

```bash
git push origin --delete js/io/packet
```

**6. Remove the local branch (optional)**

```bash
git branch -D js/io/packet
```

**7. Remove related local files (optional)**

Remove any temporary files or other local artifacts associated with the feature. If a separate Git worktree was used, remove it with:

```bash
git worktree remove <path>
```

Be careful not to remove files that are still needed.

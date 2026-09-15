# Linux Real-Time Permissions for User Processes

> *Summarized by DeepSeek AI*  
> *Based on standard Linux real-time configuration practices*

## Overview

To allow a user program to set real-time priorities (`SCHED_FIFO`, `SCHED_RR`, or `SCHED_DEADLINE`), special permissions are required. Standard user accounts cannot set real-time policies without explicit configuration.

## Quick Setup

Choose **one** of the following methods:

### Method 1: Capability (Single Binary)

Grant the `CAP_SYS_NICE` capability directly to your executable:

```bash
sudo setcap cap_sys_nice+ep /path/to/your/program
```

Verify with:

```bash
getcap /path/to/your/program
```

### Method 2: PAM Limits (User/Group)
Edit /etc/security/limits.conf:

```bash
sudo nano /etc/security/limits.conf
```

Add these lines:

```
@yourgroup   soft    rtprio   99
@yourgroup   hard    rtprio   99
```

Then add your user to the group:

```bash
sudo usermod -aG yourgroup $USER
```

Log out and back in for changes to take effect.

### Method 3: Both (Recommended)
Apply both methods. The system enforces the stricter limit.

## Testing Your Configuration

Run this command as your user:

```bash
chrt -f 50 sleep 1
```

## Systemd Services

For programs running as systemd user services, add to your .service file:

```ini
[Service]
LimitRTPRIO=99
```

## Important Gotchas

| Issue | Note |
|-------|------|
| **Group changes** | Requires logout/login to take effect |
| **Containers** | `CAP_SYS_NICE` may not work inside user namespaces |
| **Kernel limits** | Check `sysctl kernel.sched_rt_runtime_us` - if `0`, RT scheduling is disabled for non-root |
| **Cgroups** | `CPUQuota=` settings can conflict with real-time priorities |

## Priority Ranges

| Policy | Priority Range | Notes |
|--------|---------------|-------|
| `SCHED_FIFO` / `SCHED_RR` | 1 (lowest) - 99 (highest) | Requires `CAP_SYS_NICE` |
| `SCHED_DEADLINE` | N/A (uses runtime/deadline/period) | Requires `CAP_SYS_NICE` |
| `SCHED_OTHER` / `SCHED_BATCH` | Uses nice values (-20 to 19) | No special permissions needed |
| `SCHED_IDLE` | N/A (always idle) | No special permissions needed |

## Additional Resources
- `man 7 sched` - Linux scheduling documentation
- `man 2 sched_setattr` - Real-time policy system calls
- `man 1 chrt` - Command-line real-time testing tool

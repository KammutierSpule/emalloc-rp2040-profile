# emalloc-rp2040-profile
Profile emalloc library using RP2040

## Example Alloc5Release3_* test
systicks (1 tick = 1/125000000 second)

### Performance Metrics

#### emalloc.alloc (cycles)
| Size (bytes) | Min | Avg   | Max    |
|--------------|-----|-------|--------|
| 16           | 162 | 4174  | 27510  |
| 64           | 164 | 3255  | 20979  |
| 128          | 175 | 2115  | 13771  |
| 256          | 164 | 1620  | 54326  |
| 512          | 164 | 1093  | 21515  |

#### emalloc.free (cycles)
| Size (bytes) | Min | Avg   | Max     |
|--------------|-----|-------|---------|
| 16           | 207 | 2213  | 125723  |
| 64           | 207 | 3891  | 88716   |
| 128          | 207 | 2925  | 60008   |
| 256          | 207 | 2541  | 45868   |
| 512          | 207 | 1754  | 24373   |

#### emalloc.alloc (microseconds)
| Size (bytes) | Min  | Avg  | Max   |
|--------------|------|------|-------|
| 16           | 1.3  | 33.4 | 220.1 |
| 64           | 1.3  | 26.0 | 167.8 |
| 128          | 1.4  | 16.9 | 110.2 |
| 256          | 1.3  | 13.0 | 434.6 |
| 512          | 1.3  | 8.7  | 172.1 |

#### emalloc.free (microseconds)
| Size (bytes) | Min  | Avg  | Max    |
|--------------|------|------|--------|
| 16           | 1.7  | 17.7 | 1005.8 |
| 64           | 1.7  | 31.1 | 709.7  |
| 128          | 1.7  | 23.4 | 480.1  |
| 256          | 1.7  | 20.3 | 366.9  |
| 512          | 1.7  | 14.0 | 195.0  |

### #comparison

freertos.alloc average ~308 cycles (2.5us)
freertos.free average ~341 cycles (2.7us)

libc.alloc average ~393 cycles (3.0us)
libc.free average ~350 cycles (2.8us)

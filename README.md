# pomidor

Build and run:
```bash
pio run --target upload && pio device monitor --baud 115200
```

Build and run (from scratch):
```bash
pio run --target clean && pio run --target upload && pio device monitor --baud 115200
```
[link1](https://betterstack.com/community/guides/logging/logging-in-go/#best-practices-for-writing-and-storing-go-logs)

sloglint

```.golangci.yaml
linters-settings:
  sloglint:
    # Enforce not mixing key-value pairs and attributes.
    # Default: true
    no-mixed-args: false
    # Enforce using key-value pairs only (overrides no-mixed-args, incompatible with attr-only).
    # Default: false
    kv-only: true
    # Enforce using attributes only (overrides no-mixed-args, incompatible with kv-only).
    # Default: false
    attr-only: true
    # Enforce using methods that accept a context.
    # Default: false
    context-only: true
    # Enforce using static values for log messages.
    # Default: false
    static-msg: true
    # Enforce using constants instead of raw keys.
    # Default: false
    no-raw-keys: true
    # Enforce a single key naming convention.
    # Values: snake, kebab, camel, pascal
    # Default: ""
    key-naming-case: snake
    # Enforce putting arguments on separate lines.
    # Default: false
    args-on-sep-lines: true

```

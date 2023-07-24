# File pickup plugin

## Installation

Follow the [instructions](https://docs.halon.io/manual/comp_install.html#installation) in our manual to add our package repository and then run the below command.

### Ubuntu

```
apt-get install halon-extras-file-pickup
```

### RHEL

```
yum install halon-extras-file-pickup
```

## Configuration

### smtpd.yaml

For the configuration schema, see [file-pickup.schema.json](file-pickup.schema.json).

```
plugins:
  - id: file-pickup
    config:
      directories:
        - id: foo
          path: /foo
          serverid: foo # Should be of type "plugin"
          threads: 1
          concurrency: 0 # Unlimited
```
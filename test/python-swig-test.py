#!/usr/bin/env python3

import deepstream

expected = "0.1.0"
version = deepstream.version_to_string()

if deepstream.version_to_string() != expected:
    raise ValueError("Version mismatch; expected {}, got {}".format(expected, version))






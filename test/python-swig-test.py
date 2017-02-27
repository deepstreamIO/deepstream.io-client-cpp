#!/usr/bin/env python3

import deepstream

version = deepstream.version_to_string()

if deepstream.version_to_string() != "0.1.0":
    raise Exception("Version mismatch; expected 0.0.1, got {}".format(version))






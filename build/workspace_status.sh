#!/bin/bash

echo "STABLE_GIT_DESCRIBE $(git --no-pager describe --tags --always --dirty | sed -e s@version/@@)"

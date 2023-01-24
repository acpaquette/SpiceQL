#!/bin/sh

# Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.

export AWS_LAMBDA_FUNCTION_TIMEOUT=10000

# need this because of the library installing in lib64, TODO: make it install in just lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/var/lang/lib64/

export _HANDLER="$1"
echo $_HANDLER

RUNTIME_ENTRYPOINT=/var/runtime/bootstrap
if [ -z "${AWS_LAMBDA_RUNTIME_API}" ]; then
  exec /usr/local/bin/aws-lambda-rie $RUNTIME_ENTRYPOINT
else
  exec $RUNTIME_ENTRYPOINT
fi

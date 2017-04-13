#!/usr/bin/env bash
#
# Script to update necessary values in the Doxyfile, and commence the build of the documentation.
#
# Copyright (C) 2017 Gerad Munsch <gmunsch@unforgivendevelopment.com>
#
#

# DEFINE SOME IMPORTANT CONSTANTS #
PRJ_ROOT_DIR="$(pwd)/.."
PRJ_DOC_DIR="${PRJ_ROOT_DIR}/docs"
PRJ_DOXYFILE="${PRJ_ROOT_DIR}/Doxyfile"
PRJ_LIB_PROPS="${PRJ_ROOT_DIR}/library.properties"

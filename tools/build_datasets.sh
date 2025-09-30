#!/bin/bash
set -e

pipenv install --deploy --ignore-pipfile
pipenv run python generate_data.py "$@"

echo "Datasets generated! (JSONs, PNG atlases, map)"

#!/bin/bash

echo "Deleting the build folder..."
rm -r build

echo "Creating a build folder..."
mkdir build && cd build

echo "Running CMake..."
cmake ..

echo "Running Make..."
make

echo "Done..."

#!/usr/bin/env sh

echo "Building elf2sun"
gcc elf2sun.c -o elf2sun

if [ ! -x "./gcc_wrapper.sh" ]; then
  echo "Error: gcc_wrapper.sh not found or not executable."
  exit 1
fi

# Iterate over subdirectories in programs/
for dir in programs/*/; do
  if [ -d "$dir" ]; then

  echo "Building $(basename $dir)"

  # If program has custom build.sh
  if [ -f "$dir/build.sh" ]; then
    (cd "$dir" && bash build.sh)
  
  # No build.sh but main.c -> default to wrapper
  elif [ -f "$dir/main.c" ]; then
    (cd "$dir" && "../../gcc_wrapper.sh" main.c)

  else
    echo "No build.sh or main.c found in $dir, skipping..."
    fi
  fi
done

# Run elf2sun with all .out files in build/
echo "Running elf2sun with output files in build/"
./elf2sun build/*.out

# To build in 32 bit you need multilib gcc
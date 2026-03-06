#bin/sh

OS=$(ls ../bin)

echo run the lmbench on $OS...
echo
env OS=$OS ./config-run
env OS=$OS ./results

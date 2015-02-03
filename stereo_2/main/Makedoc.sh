#!/bin/sh
#
# This script cats all the source files together and creates a contents list
# The resulting file is called all.txt and can be printed out.

mkdir all
cp *.[ch] ./all
cd all
indent -kr -pcs -ts1 *.[ch]
grep -n -v -h "@@@@@@@@" *.[ch] | grep -n -v -h "@@@@@@@@" | sed 's/	/        /g'> source.txt
grep -h "\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*\*" -A 2 source.txt | \
grep -h "\/\*\ " | sed 's/\/\*//g' | sed 's/\*\///g' > contents.txt
echo -e '\n\n\n\n' >> contents.txt
cat contents.txt source.txt > ../all.txt
cd ..
rm -r ./all
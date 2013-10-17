# Event-Based Branch Facility Tests.  Save area environment variable test.
#
# Copyright IBM Corp. 2013
#
# The MIT License (MIT)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
# Contributors:
#     IBM Corporation, Adhemerval Zanella - Initial implementation.

BINDIR="${BINDIR:-.}"
EBB_LIB="${1:-$BINDIR}"/ebb/.libs/libpaf-ebb.so

if [ "x$1" = "x-h" -o "x$1" = "x--help" ]; then
  echo "USAGE: $0"
  exit 1
fi

function save_area()
{
  LIBPAF_EBB_SAVE_AREA=$1 $EBB_LIB | awk " \
  { \
    if (/Context/) { \
      split(\$0, x, \":\"); \
      gsub(/[[:space:]]+/, \"\", x[2]); \
      print x[2] \
    } \
  }"
}

save_types=(tls TLS tcb TCB)
for type in ${save_types[@]}
do
  value=$(save_area $type)
  if [ $value != "${type^^}" ]; then
    echo "FAILED"
    echo "LIBPAF_EBB_SAVE_AREA=$type $EBB_LIB returned $value"
    exit 1
  fi
done

echo "PASS"

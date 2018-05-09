set -e

cp old.scanner/stdlib/caffe/* /opt/scanner/stdlib/caffe
cd /opt/scanner
./build.sh
cd /app

set -e

mkdir data
wget https://raw.githubusercontent.com/junyanz/light-field-video/master/data/download_lfv.sh
chmod +x download_lfv.sh
./download_lfv.sh cats

ffmpeg -y -framerate 30 -i data/cats/lightfield/frame_%04d.jpg 1.mp4
ffmpeg -y -i "concat:1.mp4|1.mp4|1.mp4" -r 30 -g 30 -t 00:01:30 cats.mp4

rm download_lfv.sh
rm 1.mp4
rm -rf data

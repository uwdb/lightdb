./createtilescript.py $1/test-1K-20s.h264 1024 512 4 4 20 30 > 4x4-1K-20s.sh
./createtilescript.py $1/test-1K-40s.h264 1024 512 4 4 40 30 > 4x4-1K-40s.sh
./createtilescript.py $1/test-1K-60s.h264 1024 512 4 4 60 30 > 4x4-1K-60s.sh

./createtilescript.py $1/test-2K-20s.h264 2048 1024 4 4 20 30 > 4x4-2K-20s.sh
./createtilescript.py $1/test-2K-40s.h264 2048 1024 4 4 40 30 > 4x4-2K-40s.sh
./createtilescript.py $1/test-2K-60s.h264 2048 1024 4 4 60 30 > 4x4-2K-60s.sh

./createtilescript.py $1/test-4K-20s.h264 3840 1920 4 4 20 30 > 4x4-4K-20s.sh
./createtilescript.py $1/test-4K-40s.h264 3840 1920 4 4 40 30 > 4x4-4K-40s.sh
./createtilescript.py $1/test-4K-60s.h264 3840 1920 4 4 60 30 > 4x4-4K-60s.sh

chmod +x 4x4-1K-20s.sh
chmod +x 4x4-1K-40s.sh
chmod +x 4x4-1K-60s.sh

chmod +x 4x4-2K-20s.sh
chmod +x 4x4-2K-40s.sh
chmod +x 4x4-2K-60s.sh

chmod +x 4x4-4K-20s.sh
chmod +x 4x4-4K-40s.sh
chmod +x 4x4-4K-60s.sh


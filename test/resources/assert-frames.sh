ffprobe -v error -count_frames -select_streams v:0 \
        -show_entries stream=nb_read_frames -of default=nokey=1:noprint_wrappers=1 \
        $1 \
        | grep -w $2 > /dev/null

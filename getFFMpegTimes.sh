for var in "$@"
do
    T="$(date +%s%M)"
    ffmpeg -y -i "$var" -c libx264 output.h264 &>/dev/null
    T="$(($(date +%s%M)-T))"
    echo "$var CPU time: ${T}"
    T="$(date +%s%M)"
    ffmpeg -y -i "$var" -c nvenc_h264 output.h264 &>/dev/null
    T="$(($(date + %s%M)-T))"
    echo "$var CPU time: ${T}"
done

    

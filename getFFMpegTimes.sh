for var in "$@"
do
    T="$(date +%s%3N)"
    ffmpeg -y -i "$var" -c libx264 output.h264 &>/dev/null
    T="$(($(date +%s%3N)-T))"
    echo "$var CPU time: ${T}"
    T="$(date +%s%3N)"
    ffmpeg -y -i "$var" -c nvenc_h264 output.h264 &>/dev/null
    T="$(($(date +%s%3N)-T))"
    echo "$var GPU time: ${T}"
done

    

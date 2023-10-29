echo "info mem" | socat - unix-connect:qemu-monitor-socket 2>/dev/null | tail -n +5 | head -n -1 | cut -d' ' -f1,2,3

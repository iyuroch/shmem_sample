./main &
./main &

wait $(jobs -p)
sleep 0.1

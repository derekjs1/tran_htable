
for i in 1 2 4 8
do
    echo "Threads: $i. Lookup: $j %"
    echo "./mainSSB64 -p $i -R $j"
    ./mainSSB64 -p $i -R $j
done

	
for i in 1 2 4 8
do
    echo "Threads: $i. Lookup: $j %"
    echo "./HashBenchSSB64 -p $i -R $j"
    ./HashBenchSSB64 -p $i -R $j
done


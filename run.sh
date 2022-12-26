if [ $1 -eq 1 ] 
then 
    rm -f data.db
fi
make ht
./build/ht_main
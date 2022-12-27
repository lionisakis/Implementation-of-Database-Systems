if [ $1 -eq 1 ] 
then 
    rm -f data.db
    rm -f index.db
fi
make sht
./build/sht_main
#! /bin/bash
# dump databases

echo "Begin SQLITE database dump"
echo "$PWD"
cd ./databases
sqlite3 < ../dump_databases.cmd


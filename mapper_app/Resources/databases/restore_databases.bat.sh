#! /bin/bash
# restore databases
echo "Begin SQLITE database dump"
echo "$PWD"
cd ./databases
sqlite3 < ../restore_databases.cmd

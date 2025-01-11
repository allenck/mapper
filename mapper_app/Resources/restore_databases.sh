#! /bin/bash
# restore databases
cd databases
sqlite3 < ../restore_databases.cmd

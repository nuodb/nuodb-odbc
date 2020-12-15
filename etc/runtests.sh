#!/bin/sh
#
# (C) Copyright NuoDB, Inc. 2020  All Rights Reserved.
#
# This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
# See the LICENSE file provided with this software.
#
# Start a test DB for use with NuoODBCTest
# then run the unit tests

dbname=NuoODBCTestDB
dbauser=dba
dbapwd=dba

# ---- Setup

die () { echo "$*" 1>&2; exit 1; }

usage () { echo "$*"; die "usage: $0 <installdir> <testexe>"; }

instdir=$1
test=$2
shift 2

[ -n "$test" ] || usage

sofile="$instdir/lib64/libNuoODBC.so"

[ -f "$sofile" ] \
    || usage "NuoDB ODBC driver (lib64/libNuoODBC.so) not found in $instdir"

[ -x "$test" ] || die "Test $test does not exist."

command -v nuocmd >/dev/null 2>&1 \
    || die 'Cannot locate NuoDB AP client (nuocmd).  Set your $PATH.'

# ---- Cleanup

tmpdir=
dbstarted=false

cleandir() {
    e=$?
    if [ $e -ne 0 ]; then
        if $dbstarted; then
            echo "Failed:$e: Leaving DB running.  Test output in $tmpdir"
            nuocmd show domain 2>/dev/null
        fi
    else
        echo "Success: Cleaning up"
        deletedb "$dbname"
        [ -n "$tmpdir" ] && rm -rf "$tmpdir"
    fi
}

trap cleandir 0

# ---- Manage AP

# See if nuoadmin is running; if not start it
checkap () {
    last=$(nuocmd get servers 2>&1)
    if [ $? -ne 0 ]; then
        # Look for some common problems
        case "$last" in
            (*SSL:\ WRONG_VERSION_NUMBER*)
                die "TLS error?  AP started without TLS, or mismatched key" ;;
            (*BadStatusLine*)
                die "TLS error?  AP requires TLS: configure nuocmd with proper keys" ;;
        esac
        die "Failed to connect to the NuoDB Admin Process (nuocmd get servers)"
    fi
}

# ----- Manage DB

# We should be using json output but it would mean relying on jq or something
get_dbstate () {
    nuocmd show database --db-name "$1" 2>/dev/null \
        | sed -n "s/.* $1 .*state = \([A-Z_]*\).*/\1/p"
}

get_archiveids () {
    nuocmd show archives --db-name "$1" --archive-format 'arid={id}' \
        | sed -n 's/^arid=//p'
}

deletedb () {
    # If the DB doesn't exist nothing to do
    state=$(get_dbstate "$1")
    case $state in
        (''|TOMBSTONE) return ;;
    esac

    echo "Deleting database $1"

    # Get archives for this directory
    arids=$(get_archiveids "$1")

    [ "$state" = NOT_RUNNING ] \
        || nuocmd shutdown database --db-name "$1" \
        || die "Failed to shut down running database $1"

    # Wait for all engines to exit
    nuocmd check database --db-name "$1" --num-processes 0 --timeout 30 \
        || die "$1: Timed out waiting to shut down"

    nuocmd delete database --db-name "$1" \
        || die "Failed to delete database $1"

    for id in $arids; do
        nuocmd delete archive --archive-id "$id" --purge \
            || die "Failed to delete archive ID $id"
    done
}

createdb () {
    echo "Creating database $1 ..."

    # Create the database
    nuocmd create archive --server-id nuoadmin-0 --db-name "$1" \
           --archive-path "$tmpdir/$1-1" \
        || die "$1: Failed to create an archive at $tmpdir/$1-1"

    nuocmd create database --db-name "$1" --te-server-ids nuoadmin-0 \
           --dba-user "$dbauser" --dba-password "$dbapwd" \
        || die "$1: Failed to create database"

    nuocmd check database --db-name "$1" --check-running --timeout 30 --num-processes 2 \
        || die "$1: Failed to go RUNNING"

    dbstarted=true

    nuocmd show domain

    echo "Started:"
    echo "  DB Name:      $1"
    echo "  DBA User:     $dbauser"
    echo "  DBA Password: $dbapwd"
}

# Set up the temporary directory
tmpdir=$(mktemp -d -t "$dbname.XXXXXXXX") || die "Failed to create a temp dir!"

checkap
deletedb "$dbname"
createdb "$dbname"

# Test the local library not the installed library
NUOODBC_TEST_TEMP="$tmpdir" \
NUOODBC_LIB="$sofile" \
NUODB_DBNAME="$dbname" NUODB_USER="$dbauser" NUODB_PASSWORD="$dbapwd" \
    "$test" "$@" || die "Test failed"

nuocmd show domain

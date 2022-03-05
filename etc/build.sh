#!/bin/sh
# (C) Copyright 2022 NuoDB, Inc.  All Rights Reserved.
#
# This software is licensed under the MIT License EXCEPT WHERE OTHERWISE NOTED!
# See the LICENSE file provided with this software.

CMD=${0##*/}
BASE=$(cd "${0%$CMD}.." && pwd)

die () { echo "$*"; exit 1; }

case $1 in
    (-h|--help) cat <<'EOF'
Build the NuoDB ODBC driver.

By default, download the latest NuoDB Server package to retrieve the
NuoDB C++ driver and build against it.

Behaviors can be modified via various environment variables:

BUILDTEMP    : Temporary directory to use for builds.
               If not set, one is created.

NUODB_HOME   : An existing NuoDB Server installation.
               If not set, download the latest and unpack it.

SERVER_PKG   : An already-downloaded NuoDB Server package.
               If not set, download the package.

BUILD_TYPE   : Type of build.
               Default is RelWithDebInfo.

ODBC_INCLUDE : Location of the unixODBC headers and library.
ODBC_LIB     : Default assumes they are in the default locations.

CXX          : C++ and C compilers.
CC           : By default uses the compilers cmake discovers.

CFG_ARGS     : Extra arguments to pass to cmake configuration.
               Default is empty.

BLD_ARGS     : Extra arguments to pass to cmake build.
               Default is '-- -j8 -k'.

The results will be installed into $BUILDTEMP.
EOF
                exit 0
                ;;
    ('') : ok ;;
    (*) die "usage: $0 [-h|--help]" ;;
esac

download=https://ce-downloads.nuohub.org
arch=linux.$(uname -m)

tmpdir=${BUILDTEMP:-$(mktemp -t -d nuo.XXXXXX)}
mkdir -p "$BUILDTEMP"

if test -z "$NUODB_HOME"; then
    echo ".. Retrieving NuoDB Server version list"
    curl -s -o "$tmpdir"/ver.txt "$download/supportedversions.txt" \
        || die "Failed to download NuoDB server versions description"

    ver=$(tail -n1 "$tmpdir"/ver.txt)

    if test -z "$SERVER_PKG"; then
        echo ".. Retrieving NuoDB Server version $ver"
        curl -s -o "$tmpdir"/nuodb.tgz "$download/nuodb-$ver.$arch.tar.gz" \
            || die "Failed to download NuoDB server version $ver"
        SERVER_PKG="$tmpdir"/nuodb.tgz
    fi

    echo ".. Unpacking NuoDB Server version $ver"
    rm -rf "$tmpdir"/nuodb
    mkdir -p "$tmpdir"/nuodb
    (cd "$tmpdir"/nuodb && tar xzf "$SERVER_PKG" --strip-components 1) \
        || die "Failed to unpack NuoDB server release"

    export NUODB_HOME="$tmpdir"/nuodb
fi

fullver=$("$NUODB_HOME"/bin/nuodb --version) \
    || die "Cannot run nuodb executable"

echo ".. Building against $fullver"
rm -rf "$tmpdir"/obj "$tmpdir"/dist
mkdir -p "$tmpdir"/obj "$tmpdir"/dist

(cd "$tmpdir"/obj \
     && cmake "$BASE" \
              -DNUODB_HOME="$NUODB_HOME" \
              -DCMAKE_INSTALL_PREFIX="$tmpdir/dist" \
              -DCMAKE_BUILD_TYPE=${BUILD_TYPE:-RelWithDebInfo} \
              ${ODBC_INCLUDE:+-DODBC_INCLUDE="$ODBC_INCLUDE"} \
              ${ODBC_LIB:+-DODBC_LIB="$ODBC_LIB"} \
              ${CC:+-DCMAKE_C_COMPILER=$CC} \
              ${CXX:+-DCMAKE_CXX_COMPILER=$CC} \
              $CFG_ARGS \
     && cmake --build . --target install ${BLD_ARGS:- -- -j8 -k}) \
    || die "Failed to build NuoDB ODBC"

odbcver=$(cat "$tmpdir/obj/etc/version.txt") \
    || die "Cannot determine NuoDB ODBC driver version"

dirnm="nuodbodbc-$odbcver.$arch"
echo ".. Creating $tmpdir/$dirnm.tar.gz"
mv "$tmpdir"/dist "$tmpdir/$dirnm"
(cd "$tmpdir" && tar czf "$dirnm.tar.gz" "$dirnm") \
    || die "Failed to create $tmpdir/$dirnm.tar.gz"

echo "Build complete:"
echo "    Directory: $tmpdir/$dirnm"
echo "    Package:   $tmpdir/$dirnm.tar.gz"

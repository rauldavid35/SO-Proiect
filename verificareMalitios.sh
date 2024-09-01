chmod +r "$1"

result=0
SAFE="SAFE"

if grep -q "corrupted" "$1"; then
    result=1
fi
if grep -q "dangerous" "$1"; then
    result=1
fi
if grep -q "risk" "$1"; then
    result=1
fi
if grep -q "attack" "$1"; then
    result=1
fi
if grep -q "malware" "$1"; then
    result=1
fi
if grep -q "malicious" "$1"; then
    result=1
fi

if grep -q "[0x80-0xFF]" "$1"
then
    result=1
fi

if test $result -eq 1
then
    echo "$1"
else
    echo "$SAFE"
fi

chmod -r "$1"

exit $result
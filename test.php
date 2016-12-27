<?php
dl('flakeid.so');

function echoBar() {
    echo PHP_EOL . '===============================' . PHP_EOL;
}


echoBar();
echo 'mac: ' . PHP_EOL;
$mac_raw = flakeid_get_mac(true);
$mac = flakeid_get_mac();
// var_dump($mac);
var_dump(unpack('H*', $mac_raw));
var_dump($mac);
echoBar();

echoBar();
echo 'ipv4: ' . PHP_EOL;
$ipv4_raw = flakeid_get_ipv4(true);
$ipv4 = flakeid_get_ipv4();

// var_dump($mac);
var_dump(unpack('H*', $ipv4_raw));
var_dump($ipv4);
echoBar();

echoBar();
echo 'generate seq: ' . PHP_EOL;

for ($i = 0; $i < 10; ++$i) {
    var_dump(flakeid_next_seq((int)(gettimeofday(true) * 1000)));
}

echo "sleep 1 ms" . PHP_EOL;
usleep(1000);

for ($i = 0; $i < 10; ++$i) {
    var_dump(flakeid_next_seq((int)(gettimeofday(true) * 1000)));
}

echoBar();

echoBar();
echo 'generate flakeids: ' . PHP_EOL;

for ($i = 0; $i < 10000; ++$i) {
    var_dump(flakeid_generate(false, '-'));
    var_dump(flakeid_generate64(false));
}

echoBar();

<?php
dl('flakeid.so');

for ($i = 0; $i < 100; ++$i) {
    var_dump(flakeid_generate(false, '-'));
    var_dump(flakeid_generate64(false));
    $data = flakeid_generate64(true);
    $r = unpack('H16', strrev($data));
    var_dump($r);
    var_dump(hexdec($r));
}

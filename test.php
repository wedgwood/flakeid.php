<?php
dl('flakeid.so');

for ($i = 0; $i < 100; ++$i) {
    var_dump(flakeid_generate(false, '-'));
}

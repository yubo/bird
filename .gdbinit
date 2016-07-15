file ./bin/bird
set args -c ./etc/bird.conf -d
#b ospf_sk_open
#b ospf_rx_hook
#b ospf_send_to
b ospf_flood_lsupd
b ospf_enqueue_lsa
run

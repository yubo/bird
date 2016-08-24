file ./bin/bird
set args -c ./etc/bird.conf -d
#b ospf_sk_open
#b ospf_rx_hook
#b ospf_send_to
#b ospf_start
#b protos_commit
#b iface_patt_match
#b ospf_start
#b proto_rethink_goal
#b ospf_area_add
#b ospf_walk_matching_iface_patts
#b ospf_rx_hook
#b ospf_neighbor_new
#b packet.c:384
#b hello.c:295
#b sk_insert
#b sk_cb
b config_commit
run

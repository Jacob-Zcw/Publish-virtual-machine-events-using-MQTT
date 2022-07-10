all:libvirt_event_publish libvirt_event_subscribe

libvirt_event_publish:libvirt_event_publish.c
	gcc libvirt_event_publish.c -o libvirt_event_publish -lmosquitto -lpthread -lvirt

libvirt_event_subscribe:libvirt_event_subscribe.c
	gcc libvirt_event_subscribe.c -o libvirt_event_subscribe -lmosquitto

clean:
	rm -f libvirt_event_publish libvirt_event_subscribe

from websocket_server import WebsocketServer
import sys, getopt

# Called for every client connecting (after handshake)
def new_client(client, server):
	print("New client connected and was given id %d" % client['id'])
	server.send_message_to_all("Hey all, a new client has joined us", includeSelf=False, sender=client)


# Called for every client disconnecting
def client_left(client, server):
	print("Client(%d) disconnected" % client['id'])


# Called when a client sends a message
def message_received(client, server, message):
	print("Client(%d) said: %s" % (client['id'], message))
	server.send_message_to_all(message, includeSelf=False, sender=client)


def main(argv):
	host = ""
	port = ""
	try:
		opts, args = getopt.getopt(argv, "hs:p:", ["host=", "port="])
	except getopt.GetoptError:
		print("server.py -s <host> -p <port>")
		sys.exit(2)
	for opt, arg in opts:
		if opt == '-h':
			print("server.py -s <host> -p <port>")
			sys.exit()
		elif opt in ("-s", "--host"):
			host = arg
		elif opt in ("-p", "--port"):
			port = arg
	if host == "" or port == "":
		print("server.py -s <host> -p <port>")
		sys.exit(2)

	server = WebsocketServer(host=host, port=int(port))
	server.set_fn_new_client(new_client)
	server.set_fn_client_left(client_left)
	server.set_fn_message_received(message_received)
	server.run_forever()


if __name__ == "__main__":
	main(sys.argv[1:])

import socketserver
import tempfile
import subprocess

DEBUGD_HEADER = "DEBUGD"
PROFILE_HEADER = f"{DEBUGD_HEADER}\nPROFILE\n"

class DebugDaemonHandler(socketserver.BaseRequestHandler):
    def handle(self):
        # Receive whole request into a string
        outstr = ""
        while True:
            data = self.request.recv(1024)
            outstr += str(data, encoding="utf-8")
            if data[-1] == 0:
                break

        # Check validity
        if not outstr.startswith(DEBUGD_HEADER):
            print(f"Received invalid header from {self.client_address[0]}")
        if not outstr.startswith(PROFILE_HEADER):
            print(f"Received invalid request {outstr.splitlines()[1]} from {self.client_address[0]}")
        
        # Write to tmp file and open speedscope
        print(f"Received profile from {self.client_address[0]}")
        out = tempfile.NamedTemporaryFile(delete=False)
        out.write(outstr[len(PROFILE_HEADER):].encode("utf-8"))
        out.close()
        subprocess.run(["speedscope", out.name])


if __name__ == "__main__":
    HOST, PORT = "localhost", 59336

    with socketserver.TCPServer((HOST, PORT), DebugDaemonHandler) as server:
        # Activate the server; this will keep running until you
        # interrupt the program with Ctrl-C
        print(f"Running duckOS debugd on {HOST}:{PORT}")
        server.serve_forever()


# taken from http://www.piware.de/2011/01/creating-an-https-server-in-python/
# generate server.xml with the following command:
#    openssl req -new -x509 -keyout server.pem -out server.pem -days 365 -nodes
# run as follows:
#    python simple-https-server.py
# then in your browser, visit:
#    https://localhost:4443

import BaseHTTPServer, SimpleHTTPServer
import os
import ssl
from multiprocessing import Process


def certfile_path():
    script_dir_path = os.path.abspath(os.path.join(__file__, os.pardir))
    certificate_file_name = 'Server-localhost-sv.pem'
    return os.path.abspath(os.path.join(script_dir_path, certificate_file_name))


def port(tls_version):
    tls_version_port_map = {
        ssl.PROTOCOL_TLSv1: 4410,
        ssl.PROTOCOL_TLSv1_1: 4411,
        ssl.PROTOCOL_TLSv1_2: 4412}
    return tls_version_port_map[tls_version]


def host_tls_server(tls_version):
    httpd = BaseHTTPServer.HTTPServer(('localhost', port(tls_version)), SimpleHTTPServer.SimpleHTTPRequestHandler)
    httpd.socket = ssl.wrap_socket(httpd.socket, certfile=certfile_path(), server_side=True,
                                   ssl_version=tls_version)
    httpd.serve_forever()


if __name__ == '__main__':
    serverJobs = []
    serverJobs.append(Process(target=host_tls_server, args=(ssl.PROTOCOL_TLSv1,)))
    serverJobs.append(Process(target=host_tls_server, args=(ssl.PROTOCOL_TLSv1_1,)))
    serverJobs.append(Process(target=host_tls_server, args=(ssl.PROTOCOL_TLSv1_2,)))

    # start all the jobs
    for job in serverJobs:
        job.start()

    # wait for all the jobs to finish
    for job in serverJobs:
        job.join()

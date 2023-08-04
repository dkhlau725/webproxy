# webproxy
a basic web proxy and blocker in C

How Do I Run the Web Proxy?
  1. Make sure you are on a Linux machine - the proxy uses UNIX only libraries. The proxy was tested primarily on the school servers (linux.cpsc.ucalgary.ca) using SSH from Visual Studio Code.
  2. Use the command “hostname -i” to discover the machine’s IP. Go into a1-proxy.c and a1-blocker.c and adjust the variable “PROXY_IP” on line 5 to the IP (make sure the IP is in between the double quotes).
  3. In your browser (Firefox is recommended), change the network configuration so it connects to the proxy. In Firefox, go to Settings > Network Settings, and select Manual proxy configuration. Change the IP address to the one you set in step 2 and select any port number (ex. 6962).
  4. Compile both the proxy and the blocker in the terminal. Use “gcc a1-proxy.c -o proxy” and “gcc a1-blocker.c -o blocker”.
  5. Run the proxy first with “./proxy <port number>” (ex. ./proxy 6962) where the port number is the same as the one you set in step 3.
  6. If all goes well, the proxy will wait for the blocker to connect. Run the blocker in a separate terminal with “./blocker”. The blocker should connect to the proxy and the proxy will now wait for a browser response.
  7. Visit an HTTP site (such as http://pages.cpsc.ucalgary.ca/~carey/CPSC441/assignment1.html) and the page should display. The proxy only supports HTTP.
  8. To block content, go to the blocker terminal and type a word (ex. spongebob). If you click on the link to spongebob on the website on step 7, it should be blocked. If you don’t see the error page, refresh the page.
  9. To clear the block list, type UNBLOCK in the blocker terminal and refresh the webpage. Everything will return to normal.
  10. To exit the proxy and blocker, type EXIT in the blocker terminal. The blocker should terminate and after reloading the page, the proxy will shut off and you won’t have access to the internet anymore.
  11. If you want to relaunch the proxy, start from step 5.

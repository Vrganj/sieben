# sieben
Minecraft: Java Edition proxy written in C

The current way the proxy works is horrible.
While it does work, creating 3 threads per connection is a bad idea at high player counts.
The goal currently is to migrate to something like `libuv`.

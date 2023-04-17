#import "template.typ": *

#show: project.with(
	title: "Lab2 log",
	authors: (
		"Fanglin Xu",
	),
)

= Log

== #underline[2023.04.17]

	1. Write a article _#link("http://www.xfl.wiki/archives/30/")[How to share files between MacOS and virtual machine?]_ to make it easier to do lab.
	2. Our program should  be called like this: `./http-server --ip 127.0.0.1 --port 8888 --threads 8`, which `--ip` presents our Ip address , --port is the port that we bind with and the last is  numbers of threads. Notice there is a `--` before the argument, so long option is needed.
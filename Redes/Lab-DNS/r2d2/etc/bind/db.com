$TTL     10000
@        IN      SOA       r2d2.com. admin.r2d2.com. (
				2016090401 ;serial
				30         ;refresh
				15         ;retry
				60000      ;expire
				0          ;negative cache ttl
 				)

@                       IN      NS        r2d2.com.
r2d2                    IN      A         195.0.2.10
lavajato                IN      NS        chewbacca.lavajato.com. 
chewbacca.lavajato      IN      A        195.0.3.10


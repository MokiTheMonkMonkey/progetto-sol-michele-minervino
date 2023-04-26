#include <collector.h>
#include "./../includes/collector.h"


int sock_connect(){

    int fd_sock , cfd;

    struct sockaddr_un sa;


    ec_meno1_c(fd_sock = socket( AF_UNIX , SOCK_STREAM , 0 ) , "errore creazione socket :" , return -1 )


    memset(&sa, '\0' , sizeof(sa));

    strncpy( sa.sun_path , SOCK_NAME , SOCK_NAME_LEN );
    sa.sun_family = AF_UNIX;

    ec_meno1_c ( bind( fd_sock , (struct sockaddr *) &sa , sizeof(sa)) , "errore bind :" , return -1 )

    ec_meno1_c ( listen (fd_sock , SOMAXCONN) , "errore listen :", return -1 )

    ec_meno1_c ( cfd = accept ( fd_sock , 0 , NULL ) , "errore accept :" , return -1)

    return cfd;

}

#include "gloabl.h"
#include "protocol/IndexTSDB_protocol.h"
extern int test();
extern void sqlsacnner(char*sqlsource);

int main(int argc,char *argv[]) {

    startServer(9999);
   // test();
   /*char *sqlsource="select * from mas where a=10 and b=20 between 20:10 and 20:20";
    * char *sqlsource="insert into key values(xx,xx,xx,xx,xx,NULL)";
    char *sqlsource="select * from where conditional group by primary key";
  */
    //char *sqlsource="select * from mas where a=10 and b=20 between 20:10 and 20:20;";
   // sqlsacnner(sqlsource);


    return 0;
}

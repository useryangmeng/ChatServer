#include <mysql.h>
#include <string>
#include <iostream>

int main()
{
    MYSQL mysql;
    MYSQL_RES *res;
    MYSQL_ROW row;

    mysql_init(&mysql);

    if (!mysql_real_connect(&mysql, "127.0.0.1", "root", "513024", "chat", 3306, NULL, 0))
    {
        printf("Connect error : %s \n", mysql_error(&mysql));
    }

    const char *create_table_query =
        "CREATE TABLE IF NOT EXISTS user ("
        "id INT AUTO_INCREMENT PRIMARY KEY,"
        "name VARCHAR(50) NOT NULL UNIQUE,"
        "password VARCHAR(50) NOT NULL,"
        "state ENUM('online','offline') DEFAULT 'offline'"
        ")";

    if (mysql_query(&mysql, create_table_query))
    {
        fprintf(stderr, "CREATE TABLE user failed. Error: %s\n", mysql_error(&mysql));
        mysql_close(&mysql);
        return EXIT_FAILURE;
    }

    create_table_query =
        "CREATE TABLE IF NOT EXISTS friend ("
        "userid INT NOT NULL,"
        "friendid INT NOT NULL,"
        "PRIMARY KEY(userid,friendid)"
        ")";

    if (mysql_query(&mysql, create_table_query))
    {
        fprintf(stderr, "CREATE TABLE friend failed. Error: %s\n", mysql_error(&mysql));
        mysql_close(&mysql);
        return EXIT_FAILURE;
    }

    create_table_query =
        "CREATE TABLE IF NOT EXISTS allgroup ("
        "id INT PRIMARY KEY AUTO_INCREMENT,"
        "groupname varchar(50) NOT NULL UNIQUE,"
        "groupdesc varchar(200) DEFAULT ''"
        ")";

    if (mysql_query(&mysql, create_table_query))
    {
        fprintf(stderr, "CREATE TABLE allgroup failed. Error: %s\n", mysql_error(&mysql));
        mysql_close(&mysql);
        return EXIT_FAILURE;
    }

    create_table_query =
        "CREATE TABLE IF NOT EXISTS groupuser ("
        "groupid INT NOT NULL,"
        "userid INT NOT NULL,"
        "grouprole ENUM('creator','normal') DEFAULT 'normal',"
        "PRIMARY KEY(groupid,userid)"
        ")";

    if (mysql_query(&mysql, create_table_query))
    {
        fprintf(stderr, "CREATE TABLE groupuser failed. Error: %s\n", mysql_error(&mysql));
        mysql_close(&mysql);
        return EXIT_FAILURE;
    }

    create_table_query =
        "CREATE TABLE IF NOT EXISTS offlinemessage ("
        "userid INT NOT NULL,"
        "message VARCHAR(500) NOT NULL"
        ")";

    if (mysql_query(&mysql, create_table_query))
    {
        fprintf(stderr, "CREATE TABLE offlinemessage failed. Error: %s\n", mysql_error(&mysql));
        mysql_close(&mysql);
        return EXIT_FAILURE;
    }

    printf("all table created\n");
    mysql_close(&mysql);
    
    return 0;
}
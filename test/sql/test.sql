CREATE DATABASE IF NOT EXISTS test;

USE test;

CREATE TABLE t1 (
    ID int unsigned NOT NULL AUTO_INCREMENT PRIMARY KEY, 
    Msg varchar(70) CHARSET utf8 NULL
) ENGINE = InnoDB;



DROP TABLE IF EXISTS CloudDrive.users;

CREATE TABLE CloudDrive.users
(
    uid         int             NOT NULL PRIMARY KEY AUTO_INCREMENT,
    email       varchar(32)     NOT NULL,
    password    varchar(32)     NOT NULL
    
) ENGINE=InnoDB;

ALTER TABLE CloudDrive.users AUTO_INCREMENT=10001;

INSERT INTO CloudDrive.users(email, password) 
VALUES('nzb@qq.com', '123456');

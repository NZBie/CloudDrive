
DROP TABLE IF EXISTS CloudDrive.info;

CREATE TABLE CloudDrive.info
(
	uid             int             NOT NULL PRIMARY KEY,
	email           varchar(32)     NOT NULL,
	behalfEmail     varchar(32)     NOT NULL,
	authority       ENUM("staff", "enterprise") NOT NULL,
	nickname        char(64)        NOT NULL,
	birth           date            NULL,
	nation          varchar(64)     NULL,
	phone           char(11)        NULL,
	avatar          varchar(64)     NULL,
	background      varchar(64)     NULL,
	
	rootID      int             	NOT NULL
	
) ENGINE=InnoDB, character set = utf8;

-- INSERT INTO CloudDrive.info(email, behalfEmail, authority, nickname)
-- VALUES('nzb@cuit.com', 'nzb@qq.com', "enterprise", "nzb");

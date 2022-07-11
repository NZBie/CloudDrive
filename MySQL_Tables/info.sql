
DROP TABLE IF EXISTS CloudDrive.info;

CREATE TABLE CloudDrive.info
(
	uid             int             NOT NULL PRIMARY KEY,
	email           varchar(32)     NOT NULL,
	behalfEmail     varchar(32)     NOT NULL,
	authority       ENUM("staff", "enterprise") NOT NULL,
	nickname        char(32)        NOT NULL,
	birth           date            NULL,
	nation          varchar(32)     NULL,
	phone           char(11)        NULL,
	avatar          varchar(32)     NULL,
	background      varchar(32)     NULL,
	
	rootID      int             	NOT NULL
	
) ENGINE=InnoDB;

-- INSERT INTO CloudDrive.info(email, behalfEmail, authority, nickname)
-- VALUES('nzb@cuit.com', 'nzb@qq.com', "enterprise", "nzb");

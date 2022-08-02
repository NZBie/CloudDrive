
DROP TABLE IF EXISTS CloudDrive.folder;

CREATE TABLE CloudDrive.folder
(
    fid             int             NOT NULL PRIMARY KEY AUTO_INCREMENT,
	parID           int             NOT NULL,

    fName           varchar(128)     NOT NULL,
    size            int             NOT NULL,
	deleted         bool            NOT NULL,

    modifyTime      datetime        NULL,
    createTime      datetime        NULL,
    deleteTime      datetime        NULL

) ENGINE=InnoDB, character set = utf8;

ALTER TABLE CloudDrive.folder AUTO_INCREMENT=100001;

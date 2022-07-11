
DROP TABLE IF EXISTS CloudDrive.folder;

CREATE TABLE CloudDrive.folder
(
    fid             int             NOT NULL PRIMARY KEY AUTO_INCREMENT,
	parID           int             NOT NULL,

    fName           varchar(64)     NOT NULL,
    size            int             NOT NULL,
	deleted         bool            NOT NULL,

    modifyTime      datetime        NULL,
    createTime      datetime        NULL,
    deleteTime      datetime        NULL

) ENGINE=InnoDB;

ALTER TABLE CloudDrive.folder AUTO_INCREMENT=100001;

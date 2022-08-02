
DROP TABLE IF EXISTS CloudDrive.file;

CREATE TABLE CloudDrive.file
(
    fID             int             NOT NULL PRIMARY KEY AUTO_INCREMENT,
	parID           int             NOT NULL,

    fName           varchar(128)     NOT NULL,
    extension       varchar(16)      NOT NULL,
    size            int             NOT NULL,
    deleted         bool            NOT NULL,

    modifyTime      datetime        NOT NULL,
    createTime      datetime        NOT NULL,
    deleteTime      datetime        NULL

) ENGINE=InnoDB, character set = utf8;

ALTER TABLE CloudDrive.file AUTO_INCREMENT=200001;

-- INSERT INTO CloudDrive.info (email, behalfEmail, authority, nickname)
-- VALUES('nzb@cuit.com', 'nzb@qq.com', "enterprise", "nzb");

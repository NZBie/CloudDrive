
DROP TABLE IF EXISTS CloudDrive.upload;

CREATE TABLE CloudDrive.upload
(
    id              int             NOT NULL PRIMARY KEY AUTO_INCREMENT,
    name            varchar(128)    NOT NULL,
    md5             char(32)        NOT NULL,
    tot_size        int             NOT NULL,
    part_num        int             NOT NULL
    -- upload_state    varchar(128)    NOT NULL
    
) ENGINE=InnoDB, character set = utf8;

ALTER TABLE CloudDrive.upload AUTO_INCREMENT=300001;

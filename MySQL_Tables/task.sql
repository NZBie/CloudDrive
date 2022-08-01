
DROP TABLE IF EXISTS CloudDrive.task;

CREATE TABLE CloudDrive.task
(
    id              int             NOT NULL PRIMARY KEY,
    uid             int             NOT NULL,
    fid             int             NOT NULL
    
) ENGINE=InnoDB;

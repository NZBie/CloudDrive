TRUNCATE CloudDrive.folder;
TRUNCATE CloudDrive.file;
TRUNCATE CloudDrive.upload;
TRUNCATE CloudDrive.download;
TRUNCATE CloudDrive.task;

ALTER TABLE CloudDrive.folder AUTO_INCREMENT=100001;
ALTER TABLE CloudDrive.file AUTO_INCREMENT=200001;
ALTER TABLE CloudDrive.upload AUTO_INCREMENT=300001;

insert into CloudDrive.folder (parID, fName, size, deleted, modifyTime, createTime) value(100000, "nzb@qq.com", 0, 0, "2001-12-29 9:00", "2001-12-29 9:00");


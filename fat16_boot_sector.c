#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

/* FAT 16 BOOT SECTOR INFORMATION */
struct fat_16_boot_sect_t
{
  int bytes_per_sector;
  int sectors_per_cluster;
  int size_in_sectors_of_rsrv_area;
  int number_of_fats;
  int max_num_files_in_root_dir;
  int num_sectors_in_filesystem;
  int size_in_sectors_of_each_fat;
  int num_sectors_before_start_of_partition;
};

/* CONVERT THE 16 BITS READ TO DECIMAL */
unsigned int hex_to_dec(char *str, int num_bytes)
{
  int dec_value;
  int i;
  
  dec_value = 0;
  
  /* BIG ENDIAN */
  for (i = num_bytes; i > 0; i--)
  {
    dec_value += abs((unsigned int)str[i-1]) * (pow(16, (i-1)*2));
  }
  
  return dec_value;
}

/* READ BOOT SECTOR */
unsigned int read_boot_sector(FILE *fp, int byte_offset, int num_bytes)
{
  int bytes_in_dec;
  char *buf;
  
  if ((buf = (char *)malloc(num_bytes+1)) == NULL) /* +1 FOR NULL TERMINATOR */
  {
    perror("Malloc error\n");
    return -1;
  }
  
  /* OFFSET FROM BEGINING OF PARTITION WHERE THE INFO RESIDES */ 
  if (fseek(fp, byte_offset, SEEK_SET) != 0)
  {
    perror("Fseek error.\n");
    free(buf);
    return -1;
  }
  
  /* READ BYTES FROM BOOT SECTOR */
  if (fread(buf, 1, num_bytes, fp) != num_bytes)
  {
    perror("Fread error.\n");
    free(buf);
    return -1;
  }
  
  /* CONVERT BYTES READ INTO 16 BIT BIG ENDIAN DECIMAL REPRESENTATION */ 
  if ((bytes_in_dec = hex_to_dec(buf, num_bytes)) == -1)
  {
    free(buf);
    return -1;
  }      
  
  free(buf);
  return bytes_in_dec;
}


/* FAT 16 INIT FUNCTION */ 
int fat_16_read_boot_sector(FILE *fp,
                struct fat_16_boot_sect_t *fat_16_boot_sect_ptr)
{
  /************************
   * GET BOOT SECTOR INFO *
   ************************/
  if ((fat_16_boot_sect_ptr->bytes_per_sector = read_boot_sector(fp, 11, 2)) == -1) /* BYTES 11 AND 12 */
  {
    return -1; /* -1 FOR ERROR */
  }
  if ((fat_16_boot_sect_ptr->sectors_per_cluster = read_boot_sector(fp, 13, 1)) == -1) /* JUST BYTE 13 */
  {
    return -1;
  }
  if ((fat_16_boot_sect_ptr->size_in_sectors_of_rsrv_area = read_boot_sector(fp, 14, 2)) == -1) /* BYTES 14 AND 15 */
  {
    return -1;
  }
  if ((fat_16_boot_sect_ptr->number_of_fats = read_boot_sector(fp, 16, 1)) == -1) /* JUST BYTE 16 */
  {
    return -1;
  }
  if ((fat_16_boot_sect_ptr->max_num_files_in_root_dir = read_boot_sector(fp, 17, 2)) == -1) /* BYTES 17 AND 18 */
  {
    return -1;
  }
  /* FOR NUM SECTORS IN FS, TRY THIS FIRST, IF ZERO THEN IT CAN'T FIT IN 16BIT FORM, TRY BELOW */
  if ((fat_16_boot_sect_ptr->num_sectors_in_filesystem = read_boot_sector(fp, 19, 2)) == -1) /* BYTES 17 AND 18 */
  {
    return -1;
  }
  /* IF NUM SECTORS IN FS IS ZERO, THEN GET THE 32 BIT VALUE */
  if (fat_16_boot_sect_ptr->num_sectors_in_filesystem == 0)
  {
    if ((fat_16_boot_sect_ptr->num_sectors_in_filesystem = read_boot_sector(fp, 32, 4)) == -1) /* BYTES 17 AND 18 */
    {
      return -1;
    }
  }
  if ((fat_16_boot_sect_ptr->num_sectors_before_start_of_partition = read_boot_sector(fp, 28, 4)) == -1) /* BYTES 22 AND 23 */
  {
    return -1;
  } 
  if ((fat_16_boot_sect_ptr->size_in_sectors_of_each_fat = read_boot_sector(fp, 22, 2)) == -1) /* BYTES 22 AND 23 */
  {
    return -1;
  } 

  return 0;

}

int main(int argc, char **argv)
{
  FILE *fp;
  struct fat_16_boot_sect_t fat_16_boot_sect;
  
  /***************
   * OPEN DEVICE *
   ***************/
  if ((fp = fopen(argv[1], "rb")) == NULL)
  {
    perror("Error opening drive.\n");
    exit(-1);
  }

  /********************
   * READ BOOT SECTOR *
   ********************/
  if (fat_16_read_boot_sector(fp, &fat_16_boot_sect) == -1)
  {
    perror("Error reading FAT16 boot sector information.\n");
    exit(-1);
  }
  
  /*********************************
   * OUTPUT FAT16 BOOT SECTOR INFO *
   *********************************/
  printf("Boot Sector stats:\n");
  printf("Bytes per sector: %d\n", fat_16_boot_sect.bytes_per_sector);
  printf("Sectors per cluster: %d\n", fat_16_boot_sect.sectors_per_cluster);
  printf("Size in sectors of reserved area: %d\n", fat_16_boot_sect.size_in_sectors_of_rsrv_area);
  printf("Number of FATs: %d\n", fat_16_boot_sect.number_of_fats);
  printf("Max num files in root directory: %d\n", fat_16_boot_sect.max_num_files_in_root_dir);
  printf("Number of sectors in filesystem: %d\n", fat_16_boot_sect.num_sectors_in_filesystem);
  printf("Number of sectors before start of partition: %d\n", fat_16_boot_sect.num_sectors_before_start_of_partition);
  printf("Size in sectors of each fat: %d\n", fat_16_boot_sect.size_in_sectors_of_each_fat);

  return 0;
}


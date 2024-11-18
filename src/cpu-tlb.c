/*
 * Copyright (C) 2024 pdnguyen of the HCMC University of Technology
 */
/*
 * Source Code License Grant: Authors hereby grants to Licensee 
 * a personal to use and modify the Licensed Source Code for 
 * the sole purpose of studying during attending the course CO2018.
 */
//#ifdef CPU_TLB
/*
 * CPU TLB
 * TLB module cpu/cpu-tlb.c
 */
 
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>

int tlb_change_all_page_tables_of(struct pcb_t *proc,  struct memphy_struct * mp)
{
  /* TODO update all page table directory info 
   *      in flush or wipe TLB (if needed)
   */

  return 0;
}

int tlb_flush_tlb_of(struct pcb_t *proc, struct memphy_struct * mp)
{
  /* TODO flush tlb cached*/
  
  for(int index = 0; index < mp->maxsz;index++){
    if(mp->tlb_storage[index].pid == proc->pid){
      mp->tlb_storage[index].pid = -1;
      mp->tlb_storage[index].pgnum = -1;
      mp->tlb_storage[index].frmnumb = -1;
    }
  }
  return 0;
}

/*tlballoc - CPU TLB-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlballoc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
#ifdef IODUMP
printf("---TLB BEFORE ALLOC---: \n");
print_tlb(proc);
#endif
  int addr, val;
  
  /* By default using vmaid = 0 */
  val = __alloc(proc, 0, reg_index, size, &addr);
  if(val !=0){
    printf("---ALLOC FAILED---\n");
    return -1;
  }
  /* TODO update TLB CACHED frame num of the new allocated page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  int pgnumbers = PAGING_PAGE_ALIGNSZ(size)/PAGING_PAGESZ;
  int page_num = addr/ PAGING_PAGESZ;
  
  for(int i = 0; i< pgnumbers; i++){
  if(EXTRACT_FPN_SWAPPED_PTE(proc->mm->pgd[page_num])) continue;
  tlb_cache_write(proc->tlb,proc->pid,page_num,EXTRACT_FPN_FROM_PTE(proc->mm->pgd[page_num]),1);
  ++page_num;
  }
  
  #ifdef IODUMP
  printf("memory allocated: %d byte reg: %d\n",size,reg_index);
  printf("---TLB AFTER ALLOC---: \n");
  print_tlb(proc);

#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
  print_list_rg(proc->mm->mmap->vm_freerg_list);
#endif
  
  MEMPHY_dump(proc->mram);
#endif
  return val;
}

/*pgfree - CPU TLB-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int tlbfree_data(struct pcb_t *proc, uint32_t reg_index)
{
#ifdef IODUMP
  printf("---TLB BEFORE free:--- \n");
  print_tlb(proc);
#endif
  int size = proc->mm->symrgtbl[reg_index].rg_end - proc->mm->symrgtbl[reg_index].rg_start;
  __free(proc, 0, reg_index);
  
  /* TODO update TLB CACHED frame num of freed page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/
  int pgnumber = PAGING_PAGE_ALIGNSZ(size)/PAGING_PAGESZ;
  int addr = proc->mm->symrgtbl[reg_index].rg_start;
  int page_num = addr/PAGING_PAGESZ;
  
  for(int i = 0; i<pgnumber;i++){
  tlb_cache_write(proc->tlb,proc->pid,page_num,0,0);
  ++page_num;
  }
#ifdef IODUMP
  printf("free reg: %d\n",reg_index);
  printf("---TLB AFTER free---: \n");
  print_tlb(proc);
#ifdef PAGETBL_DUMP
 print_pgtbl(proc, 0, -1); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif
  return 0;
}


/*tlbread - CPU TLB-based read a region memory
 *@proc: Process executing the instruction
 *@source: index of source register
 *@offset: source address = [source] + [offset]
 *@destination: destination storage
 */
int tlbread(struct pcb_t * proc, uint32_t source,
            uint32_t offset, 	uint32_t destination) 
{
  BYTE data = 0;
  int frmnum = -1, val = -1, size = proc->mm->symrgtbl[source].rg_end - proc->mm->symrgtbl[source].rg_start;
	
  /* TODO retrieve TLB CACHED frame num of accessing page(s)*/
  if (size <= 0 || size <= offset){
    //printf("read region=%d offset=%d value=\n", source, offset);
    //printf("illegal read\n");
    return -1;
  }
  
  int pgnum = (proc->mm->symrgtbl[source].rg_start + offset) / PAGING_PAGESZ;
  tlb_cache_read(proc->tlb, proc->pid, pgnum, &frmnum);

	
#ifdef IODUMP
  printf("---TLB Before read---:\n");
  print_tlb(proc);
  if (frmnum >= 0)
    printf("TLB hit at read region=%d offset=%d\n", 
	         source, offset);
  else 
    printf("TLB miss at read region=%d offset=%d\n", 
	         source, offset);
#endif

  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  int TLB_miss = (frmnum < 0 ? 1 : 0); 
  
  val = __read(proc, 0, source, offset, &data);
  pg_getpage(proc->mm,pgnum,&frmnum,proc);
 
  if (val == -1) return -1;
  if(val ==-1) return -1;
  if(TLB_miss){ // TLB miss
    // chuwa co frame number
    // xu li binh thuong voi co che pagging
    // cap nhap tlb vua doc duoc tu ham __read
    tlb_cache_write(proc->tlb, proc->pid, pgnum, frmnum,1);
  }

  destination = (uint32_t) data;

  val = TLB_miss;
#ifdef IODUMP
 printf("read region=%d offset=%d value=%d\n", source, offset, (unsigned char)data);
 printf("---TLB after read---:\n");
 print_tlb(proc);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif

  MEMPHY_dump(proc->mram);
#endif

  return val;
}

/*tlbwrite - CPU TLB-based write a region memory
 *@proc: Process executing the instruction
 *@data: data to be wrttien into memory
 *@destination: index of destination register
 *@offset: destination address = [destination] + [offset]
 */
int tlbwrite(struct pcb_t * proc, BYTE data,
             uint32_t destination, uint32_t offset)
{
  int val;
  int frmnum = -1;
  int size = proc->mm->symrgtbl[destination].rg_end - proc->mm->symrgtbl[destination].rg_start;
  if (size <= 0 || size <= offset){
   // printf("write region=%d offset=%d value=%d\n", destination, offset, data);
    //printf("illegal write\n");
    return -1;
  }
  // int pgnum = PAGING_PGN((proc->sym[destination] + offset));
  int pgnum = (proc->mm->symrgtbl[destination].rg_start + offset) / PAGING_PAGESZ;
  // Đọc frame number từ TLB cache
  tlb_cache_read(proc->tlb, proc->pid, pgnum, &frmnum);

  /* TODO retrieve TLB CACHED frame num of accessing page(s))*/
  /* by using tlb_cache_read()/tlb_cache_write()
  frmnum is return value of tlb_cache_read/write value*/
#ifdef IODUMP
  // Cần in ra TLB CONTENT
 printf("---TLB Before write---:\n");
 print_tlb(proc);
  if (frmnum >= 0)
    printf("TLB hit at write region=%d offset=%d value=%d\n",
	          destination, offset, (unsigned char) data);
	else
    printf("TLB miss at write region=%d offset=%d value=%d\n",
            destination, offset, (unsigned char) data);
#endif

  /* TODO update TLB CACHED with frame num of recent accessing page(s)*/
  /* by using tlb_cache_read()/tlb_cache_write()*/

  int TLB_miss = (frmnum < 0 ? 1 : 0);
  printf("destination: %d\n",destination);
  
  val = __write(proc, 0, destination, offset, data);
  pg_getpage(proc->mm,pgnum,&frmnum,proc);
  
  if (val == -1) return -1;
  if(val ==-1) return -1;
  if(TLB_miss){ // TLB miss
    // Chưa có frame number
    // Xử lý bình thường với cơ chế paging (đã được viết trong hàm __write)
    // Sau đó cập nhật TLB với frame number mới đọc được
    tlb_cache_write(proc->tlb, proc->pid, pgnum, frmnum,1);
  }
  val = TLB_miss;

#ifdef IODUMP
  printf("---TLB after write---:\n");
  print_tlb(proc);

#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1); //print max TBL
#endif

  MEMPHY_dump(proc->mram);
#endif

  return val;
}


//#endif

#pragma mark Outlets
 
/*
 * The displayValue method rerutns a copy of _display.
 */
- (NSString *) displayValue {
   if ([_display length]) {
      return [[_display copy] autorelease];
   }
   return @"0";
}
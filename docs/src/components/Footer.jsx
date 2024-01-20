import React from 'react'
import './Footer.css';
import moment from 'moment';
import { useEffect, useState } from 'react';

function Footer() {
  const [currentYear, setCurrentYear] = useState(moment().format('YYYY'));

  useEffect(() => {
    const intervalId = setInterval(() => {
      setCurrentYear(moment().format('YYYY'));
    }, 1000);

    return () => clearInterval(intervalId);
  }, []);

  return (
    <footer>
      <p className='copyrights'>&copy; {currentYear} Clipbird. All rights reserved.</p>
    </footer>
  )
}

export default Footer
